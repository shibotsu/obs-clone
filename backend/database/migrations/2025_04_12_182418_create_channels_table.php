<?php

use App\Models\User;
use Illuminate\Database\Migrations\Migration;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Support\Facades\Schema;

return new class extends Migration
{
    /**
     * Run the migrations.
     */
    public function up(): void
    {
        Schema::create('channels', function (Blueprint $table) {
            $table->id();
            $table->foreignIdFor(User::class)->constrained()->cascadeOnDelete();
            $table->string('stream_key')->unique()->nullable();
            $table->string('title')->nullable();
            $table->text('description')->nullable();
            $table->boolean('is_live')->default(false);
            $table->string('stream_title')->nullable();
            $table->text('stream_description')->nullable();
            $table->string('stream_category')->nullable();
            $table->string('thumbnail')->nullable()->default(null);
            $table->timestamps();
        });
    }

    /**
     * Reverse the migrations.
     */
    public function down(): void
    {
        Schema::dropIfExists('channels');
    }
};
